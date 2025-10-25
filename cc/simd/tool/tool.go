package main

import (
	"archive/zip"
	"encoding/json"
	"encoding/xml"
	"errors"
	"flag"
	"fmt"
	"io"
	"net/http"
	"os"
	"path/filepath"
	"strings"
)

const (
	OWWLOAD_PAGE = "https://www.intel.com/content/www/us/en/content-details/794831/intel-intrinsics-guide-download.html"
	DOWNLOAD_URL = "https://cdrdv2.intel.com/v1/dl/getContent/794831?fileName=Intel-Intrinsics-Guide-Offline-3.6.9.zip"
)

type NodeType int

const (
	TypeText NodeType = iota
	TypeChildren
)

type Content interface {
	isContent() NodeType
}

type Text string

func (t Text) isContent() NodeType {
	return TypeText
}

type Children []*Node

func (c Children) isContent() NodeType {
	return TypeChildren
}

type Node struct {
	Name       string
	Attributes map[string]string
	Content    Content
}

func (n *Node) GetAttribute(key string) string {
	if n.Attributes != nil {
		return n.Attributes[key]
	}
	return ""
}

func (n *Node) GetNodes(name string) []*Node {
	if children, ok := n.Content.(Children); ok {
		var result []*Node
		for _, child := range children {
			if child.Name == name {
				result = append(result, child)
			}
		}
		return result
	}
	return nil
}

func (n *Node) GetText() string {
	if text, ok := n.Content.(Text); ok {
		return string(text)
	}
	return ""
}

func (n *Node) MarshalXML(e *xml.Encoder, start xml.StartElement) error {
	start.Name = xml.Name{Local: n.Name}
	if n.Attributes != nil {
		start.Attr = make([]xml.Attr, 0, len(n.Attributes))
		for key, value := range n.Attributes {
			var space, local string
			if colon := strings.Index(key, ":"); colon >= 0 {
				space = key[:colon]
				local = key[colon+1:]
			} else {
				local = key
			}
			start.Attr = append(start.Attr, xml.Attr{
				Name:  xml.Name{Space: space, Local: local},
				Value: value,
			})
		}
	}
	if err := e.EncodeToken(start); err != nil {
		return err
	}
	switch content := n.Content.(type) {
	case Text:
		text := xml.CharData(content)
		if len(text) > 0 {
			if err := e.EncodeToken(text); err != nil {
				return err
			}
		}
	case Children:
		if len(content) > 0 {
			for _, child := range content {
				if err := e.Encode(child); err != nil {
					return err
				}
			}
		}
	case nil:
		// Do Nothing
	}
	return e.EncodeToken(start.End())
}

func (n *Node) UnmarshalXML(d *xml.Decoder, start xml.StartElement) error {
	n.Name = start.Name.Local
	n.Attributes = make(map[string]string)
	for _, attr := range start.Attr {
		key := attr.Name.Local
		if attr.Name.Space != "" {
			key = attr.Name.Space + ":" + key
		}
		n.Attributes[key] = attr.Value
	}
	var (
		children Children
		text     Text
	)
	for {
		token, err := d.Token()
		if err != nil {
			return err
		}
		switch t := token.(type) {
		case xml.StartElement:
			var child Node
			if err := d.DecodeElement(&child, &t); err != nil {
				return err
			}
			children = append(children, &child)
		case xml.CharData:
			text += Text(t)
		case xml.EndElement:
			if len(children) > 0 {
				n.Content = children
			} else {
				n.Content = text
			}
			return nil
		}
	}
}

func DownloadURL(url, filepath string) error {
	response, err := http.Get(url)
	if err != nil {
		return errors.New("failed to download: " + err.Error())
	}
	defer response.Body.Close()
	if response.StatusCode != http.StatusOK {
		return errors.New("bad status: " + response.Status)
	}
	os.Remove(filepath)
	out, err := os.Create(filepath)
	if err != nil {
		return errors.New("failed to create file: " + err.Error())
	}
	defer out.Close()
	_, err = io.Copy(out, response.Body)
	if err != nil {
		return errors.New("failed to write to file: " + err.Error())
	}
	return nil
}

func ExtractZip(filename, destination string) error {
	r, err := zip.OpenReader(filename)
	if err != nil {
		return err
	}
	defer r.Close()
	for _, f := range r.File {
		fpath := filepath.Join(destination, f.Name)
		if f.FileInfo().IsDir() {
			os.MkdirAll(fpath, os.ModePerm)
			continue
		}
		if err = os.MkdirAll(filepath.Dir(fpath), os.ModePerm); err != nil {
			return err
		}
		tfile, err := os.OpenFile(fpath, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, f.Mode())
		if err != nil {
			return err
		}
		rc, err := f.Open()
		if err != nil {
			tfile.Close()
			return err
		}
		_, err = io.Copy(tfile, rc)
		tfile.Close()
		rc.Close()
		if err != nil {
			return err
		}
	}
	return nil
}

func ReadFile(filename string) string {
	data, err := os.ReadFile(filename)
	if nil != err {
		fmt.Println("Read error: ", err)
		return ""
	}
	return string(data)
}

func ProcessData(data string) {
	data = strings.TrimPrefix(strings.TrimSpace(data), "var data_js = ")
	data = strings.TrimSuffix(strings.TrimSpace(data), ";")
	data = strings.ReplaceAll(data, "\\\n", "")
	data = strings.ReplaceAll(data, "\t", "")
	if err := json.Unmarshal([]byte(data), &data); err != nil {
		fmt.Println("Error parsing intrinsics data: ", err)
		return
	}
	if err := os.WriteFile("data.xml", []byte(data), 0644); err != nil {
		fmt.Println("Error writing to data.xml:", err)
		return
	}
}

func ParseData() *Node {
	data, err := os.ReadFile("data.xml")
	if err != nil {
		fmt.Println("Error reading file: ", err)
		return nil
	}
	node := new(Node)
	err = xml.Unmarshal(data, &node)
	if err != nil {
		fmt.Println("Error unmarshaling XML:", err)
		return nil
	}
	return node
}

const (
	INTRINSICS_LIST = "intrinsics_list"
	INTRINSIC       = "intrinsic"
	CPUID           = "CPUID"
)

func ProcessIntrinc(node *Node) {
	if node == nil {
		return
	}
	if node.Name != INTRINSIC {
		return
	}
	var (
		name  = node.GetAttribute("name")
		cpuid = ""
	)
	if temp := node.GetNodes(CPUID); len(temp) > 0 {
		cpuid = temp[0].GetText()
	}
	fmt.Printf("Intrinsic: %s, CPUID: %s\n", name, cpuid)
}

func ProcessIntrincsList(node *Node) {
	if node == nil {
		return
	}
	if node.Name != INTRINSICS_LIST {
		return
	}
	if children, ok := node.Content.(Children); ok {
		fmt.Println("Number of intrincs:", len(children))
		for _, child := range children {
			ProcessIntrinc(child)
		}
	}
}

//go:generate go build -o tool.bin tool.go
func main() {
	download := flag.Bool("download", false, "download and process the Intel Intrinsics Guide")
	flag.Parse()

	if *download {
		err := DownloadURL(DOWNLOAD_URL, "Intel-Intrinsics-Guide.zip")
		if err != nil {
			fmt.Println("Download failed:", err)
			return
		}
		fmt.Println("Download complete!")
		err = ExtractZip("Intel-Intrinsics-Guide.zip", ".")
		if err != nil {
			fmt.Println("Extraction failed:", err)
			return
		}
		fmt.Println("Extraction complete!")
		data := ReadFile(filepath.Join("Intel Intrinsics Guide", "files", "data.js"))
		if len(data) > 0 {
			ProcessData(data)
		}
		os.Remove("Intel-Intrinsics-Guide.zip")
		os.RemoveAll("Intel Intrinsics Guide")
	}

	node := ParseData()
	if node != nil {
		ProcessIntrincsList(node)
	}
}
