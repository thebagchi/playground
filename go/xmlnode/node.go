package xmlnode

import (
	"bytes"
	"encoding/xml"
	"errors"
	"fmt"
	"strings"
)

// NodeType indicates the type of content in a Node
type NodeType int

const (
	TypeText NodeType = iota
	TypeChildren
)

// Content is an interface for node content (either text or children)
type Content interface {
	isContent() NodeType // Returns the type of content
}

// Text represents text content in a node (including empty content)
type Text string

func (t Text) isContent() NodeType {
	return TypeText
}

// Children represents a list of child nodes
type Children []*Node

func (c Children) isContent() NodeType {
	return TypeChildren
}

// Node represents an XML node with attributes and content
type Node struct {
	Name       string
	Attributes map[string]string
	Content    Content
}

// MakeXMLName creates an xml.Name from a string with optional namespace
func MakeXMLName(name string) xml.Name {
	var space, local string
	if colon := strings.Index(name, ":"); colon >= 0 {
		space = name[:colon]
		local = name[colon+1:]
	} else {
		local = name
	}
	return xml.Name{Space: space, Local: local}
}

// XMLNameString creates a full name string from xml.Name, including namespace if present
func XMLNameString(name xml.Name) string {
	if name.Space != "" {
		return name.Space + ":" + name.Local
	}
	return name.Local
}

// MakeName creates a colon-separated name string from namespace and local parts
func MakeName(space, local string) string {
	if space != "" {
		return space + ":" + local
	}
	return local
}

func (n *Node) GetXMLName() xml.Name {
	space := n.Attributes["xmlns"]
	return xml.Name{Space: space, Local: n.Name}
}

func PrintXMLName(name xml.Name) {
	fmt.Println(XMLNameString(name))
}

func (n *Node) GetAttribute(key string) string {
	if n == nil || n.Attributes == nil {
		return ""
	}
	if val, exists := n.Attributes[key]; exists {
		return val
	}
	return ""
}

func (n *Node) HasAttribute(key string) bool {
	if n == nil || n.Attributes == nil {
		return false
	}
	_, ok := n.Attributes[key]
	return ok
}

func (n *Node) SetAttribute(key, value string) {
	if n == nil {
		return
	}
	if n.Attributes == nil {
		n.Attributes = make(map[string]string)
	}
	n.Attributes[key] = value
}

func (n *Node) RemoveAttribute(key string) {
	if n == nil || n.Attributes == nil {
		return
	}
	delete(n.Attributes, key)
}

func (n *Node) ClearAttributes() {
	if n == nil {
		return
	}
	n.Attributes = nil
}

func (n *Node) ToXML(pretty bool) (string, error) {
	if n == nil {
		return "", nil
	}
	var buf bytes.Buffer
	encoder := xml.NewEncoder(&buf)
	if pretty {
		encoder.Indent("", "  ")
	}
	err := encoder.Encode(n)
	if err != nil {
		return "", err
	}
	return buf.String(), nil
}

func (n *Node) FromXML(data []byte) error {
	if n == nil {
		return errors.New("node is nil")
	}
	decoder := xml.NewDecoder(bytes.NewReader(data))
	return decoder.Decode(n)
}

func (n *Node) GetNodes(name string) []*Node {
	if n == nil {
		return nil
	}
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

func (n *Node) GetALL() []*Node {
	if n == nil {
		return nil
	}
	if children, ok := n.Content.(Children); ok {
		return children
	}
	return nil
}

func (n *Node) WalkNodes(fn func(*Node)) {
	if n == nil || fn == nil {
		return
	}
	if children, ok := n.Content.(Children); ok {
		for _, child := range children {
			fn(child)
		}
	}
}

func (n *Node) FindFirst(name string) *Node {
	if n == nil {
		return nil
	}
	if children, ok := n.Content.(Children); ok {
		for _, child := range children {
			if child.Name == name {
				return child
			}
		}
	}
	return nil
}

func (n *Node) HasChildren() bool {
	if n == nil {
		return false
	}
	if children, ok := n.Content.(Children); ok && len(children) > 0 {
		return true
	}
	return false
}

func (n *Node) GetText() string {
	if n == nil {
		return ""
	}
	if text, ok := n.Content.(Text); ok {
		return string(text)
	}
	return ""
}

func (n *Node) SetText(text string) {
	if n == nil {
		return
	}
	text = strings.TrimSpace(text)
	if text != "" {
		n.Content = Text(text)
	} else {
		n.Content = nil
	}
}

func (n *Node) HasText() bool {
	if n == nil {
		return false
	}
	if text, ok := n.Content.(Text); ok && len(text) > 0 {
		return true
	}
	return false
}

func (n *Node) AddChild(child *Node) {
	if n == nil || child == nil {
		return
	}
	if children, ok := n.Content.(Children); ok {
		n.Content = append(children, child)
	} else {
		n.Content = Children{child}
	}
}

func (n *Node) RemoveChild(child *Node) bool {
	if n == nil || child == nil {
		return false
	}
	if children, ok := n.Content.(Children); ok {
		for i, c := range children {
			if c == child {
				n.Content = append(children[:i], children[i+1:]...)
				return true
			}
		}
	}
	return false
}

func (n *Node) RemoveChildrenByName(name string) int {
	if n == nil {
		return 0
	}
	if children, ok := n.Content.(Children); ok {
		var (
			temp  Children
			count int
		)
		for _, c := range children {
			if c.Name != name {
				temp = append(temp, c)
			} else {
				count++
			}
		}
		n.Content = temp
		return count
	}
	return 0
}

func (n *Node) ReplaceChild(with, what *Node) bool {
	if n == nil || with == nil || what == nil {
		return false
	}
	if children, ok := n.Content.(Children); ok {
		for i, c := range children {
			if c == with {
				children[i] = what
				return true
			}
		}
	}
	return false
}

// MarshalXML implements the xml.Marshaler interface
func (n *Node) MarshalXML(e *xml.Encoder, start xml.StartElement) error {
	if n == nil {
		return nil
	}
	// Set the element name
	start.Name = xml.Name{Local: n.Name}
	if xmlns, ok := n.Attributes["xmlns"]; ok {
		start.Attr = append(start.Attr, xml.Attr{Name: xml.Name{Local: "xmlns"}, Value: xmlns})
	}

	// Add attributes
	if n.Attributes != nil {
		for key, value := range n.Attributes {
			if key != "xmlns" {
				attrName := MakeXMLName(key)
				start.Attr = append(start.Attr, xml.Attr{Name: attrName, Value: value})
			}
		}
	}

	// Start the element
	if err := e.EncodeToken(start); err != nil {
		return err
	}

	// Handle content based on type
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

	// End the element
	return e.EncodeToken(start.End())
}

// UnmarshalXML implements the xml.Unmarshaler interface
func (n *Node) UnmarshalXML(d *xml.Decoder, start xml.StartElement) error {
	// PrintXMLName(start.Name)
	n.Name = start.Name.Local
	n.Attributes = make(map[string]string)

	// Collect attributes
	for _, attr := range start.Attr {
		key := XMLNameString(attr.Name)
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
			// Handle nested elements
			var child Node
			if err := d.DecodeElement(&child, &t); err != nil {
				return err
			}
			children = append(children, &child)
		case xml.CharData:
			// Collect character data
			text += Text(t)
		case xml.EndElement:
			// Set content based on what was found
			text = Text(strings.TrimSpace(string(text)))
			if len(children) > 0 {
				n.Content = children
			} else if len(text) > 0 {
				n.Content = text
			} else {
				n.Content = nil
			}
			return nil
		}
	}
}
