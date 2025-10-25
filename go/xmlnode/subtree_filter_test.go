package xmlnode

import (
	"fmt"
	"testing"
)

const DATASTORE = `
<top xmlns="http://example.com/schema/1.2/config">
  <users>
    <user>
      <name>root</name>
      <type>superuser</type>
      <full-name>Charlie Root</full-name>
      <company-info>
        <dept>1</dept>
        <id>1</id>
      </company-info>
    </user>
    <user>
      <name>fred</name>
      <type>admin</type>
      <full-name>Fred Flintstone</full-name>
      <company-info>
        <dept>2</dept>
        <id>2</id>
      </company-info>
    </user>
    <user>
      <name>barney</name>
      <type>admin</type>
      <full-name>Barney Rubble</full-name>
      <company-info>
        <dept>2</dept>
        <id>3</id>
      </company-info>
    </user>
  </users>
</top>
`

const ENTIRE_USERS = `
<top xmlns="http://example.com/schema/1.2/config">
  <users/>
</top>
`

const ALL_USER = `
<top xmlns="http://example.com/schema/1.2/config">
  <users>
    <user/>
  </users>
</top>
`

const SELECT_ALL_NAME = `
<top xmlns="http://example.com/schema/1.2/config">
  <users>
    <user>
      <name/>
    </user>
  </users>
</top>
`

const SELECT_ALL_FOR_USER = `
<top xmlns="http://example.com/schema/1.2/config">
  <users>
    <user>
      <name>fred</name>
    </user>
  </users>
</top>
`

const SELECT_SPECIFIC_FOR_USER = `
<top xmlns="http://example.com/schema/1.2/config">
  <users>
    <user>
      <name>fred</name>
      <type/>
      <full-name/>
    </user>
  </users>
</top>
`

const SELECT_MULTIPLE = `
<top xmlns="http://example.com/schema/1.2/config">
  <users>
    <user>
      <name>root</name>
      <company-info/>
    </user>
    <user>
      <name>fred</name>
      <company-info>
        <id/>
      </company-info>
    </user>
    <user>
      <name>barney</name>
      <type>superuser</type>
      <company-info>
        <dept/>
      </company-info>
    </user>
  </users>
</top>
`

func PrintNode(node *Node) {
	data, err := node.ToXML(true)
	if err != nil {
		fmt.Println("Error: ", err)
	} else {
		fmt.Println(data)
	}
}

func TestSubtreeFilterEntireUsers(t *testing.T) {
	root := new(Node)
	if err := root.FromXML([]byte(DATASTORE)); err != nil {
		t.Error("Error: ", err)
	}
	// PrintNode(root)
	filter := new(Node)
	if err := filter.FromXML([]byte(ENTIRE_USERS)); err != nil {
		t.Error("Error: ", err)
	}
	// PrintNode(filter)
	filtered := root.SubtreeFilter(filter)
	if filtered == nil {
		t.Error("Error: filter returned empty ...")
	}
	_, err := filtered.ToXML(true)
	if err != nil {
		t.Error("Error: ", err)
	}
	PrintNode(filtered)
}

func TestSubtreeFilterAllUser(t *testing.T) {
	root := new(Node)
	if err := root.FromXML([]byte(DATASTORE)); err != nil {
		t.Error("Error: ", err)
	}
	// PrintNode(root)
	filter := new(Node)
	if err := filter.FromXML([]byte(ALL_USER)); err != nil {
		t.Error("Error: ", err)
	}
	// PrintNode(filter)
	filtered := root.SubtreeFilter(filter)
	if filtered == nil {
		t.Error("Error: filter returned empty ...")
	}
	_, err := filtered.ToXML(true)
	if err != nil {
		t.Error("Error: ", err)
	}
	PrintNode(filtered)
}

func TestSubtreeFilterSelectALLName(t *testing.T) {
	root := new(Node)
	if err := root.FromXML([]byte(DATASTORE)); err != nil {
		t.Error("Error: ", err)
	}
	// PrintNode(root)
	filter := new(Node)
	if err := filter.FromXML([]byte(SELECT_ALL_NAME)); err != nil {
		t.Error("Error: ", err)
	}
	// PrintNode(filter)
	filtered := root.SubtreeFilter(filter)
	if filtered == nil {
		t.Error("Error: filter returned empty ...")
	}
	_, err := filtered.ToXML(true)
	if err != nil {
		t.Error("Error: ", err)
	}
	PrintNode(filtered)
}

func TestSubtreeFilterSelectALLUser(t *testing.T) {
	root := new(Node)
	if err := root.FromXML([]byte(DATASTORE)); err != nil {
		t.Error("Error: ", err)
	}
	// PrintNode(root)
	filter := new(Node)
	if err := filter.FromXML([]byte(SELECT_ALL_FOR_USER)); err != nil {
		t.Error("Error: ", err)
	}
	// PrintNode(filter)
	filtered := root.SubtreeFilter(filter)
	if filtered == nil {
		t.Error("Error: filter returned empty ...")
	}
	_, err := filtered.ToXML(true)
	if err != nil {
		t.Error("Error: ", err)
	}
	PrintNode(filtered)
}

func TestSubtreeFilterSelectFewUser(t *testing.T) {
	root := new(Node)
	if err := root.FromXML([]byte(DATASTORE)); err != nil {
		t.Error("Error: ", err)
	}
	// PrintNode(root)
	filter := new(Node)
	if err := filter.FromXML([]byte(SELECT_SPECIFIC_FOR_USER)); err != nil {
		t.Error("Error: ", err)
	}
	// PrintNode(filter)
	filtered := root.SubtreeFilter(filter)
	if filtered == nil {
		t.Error("Error: filter returned empty ...")
	}
	_, err := filtered.ToXML(true)
	if err != nil {
		t.Error("Error: ", err)
	}
	PrintNode(filtered)
}

func TestSubtreeFilterSelectMultiple(t *testing.T) {
	root := new(Node)
	if err := root.FromXML([]byte(DATASTORE)); err != nil {
		t.Error("Error: ", err)
	}
	// PrintNode(root)
	filter := new(Node)
	if err := filter.FromXML([]byte(SELECT_MULTIPLE)); err != nil {
		t.Error("Error: ", err)
	}
	// PrintNode(filter)
	filtered := root.SubtreeFilter(filter)
	if filtered == nil {
		t.Error("Error: filter returned empty ...")
	}
	_, err := filtered.ToXML(true)
	if err != nil {
		t.Error("Error: ", err)
	}
	PrintNode(filtered)
}
