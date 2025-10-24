package xmlnode

import (
	"encoding/xml"
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

// MarshalXML implements the xml.Marshaler interface
func (n Node) MarshalXML(e *xml.Encoder, start xml.StartElement) error {
	// Set the element name
	start.Name = xml.Name{Local: n.Name}

	// Add attributes
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
	n.Name = start.Name.Local
	n.Attributes = make(map[string]string)

	// Collect attributes
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
			if len(children) > 0 {
				n.Content = children
			} else {
				n.Content = text
			}
			return nil
		}
	}
}
