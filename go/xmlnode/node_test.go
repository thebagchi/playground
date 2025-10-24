package xmlnode

import (
	"encoding/xml"
	"fmt"
	"testing"
)

var data = []byte(`
<root type="main" id="doc1">
	<person id="1" role="admin">
		<address type="home">
			<street>123 Main St</street>
			<city>Springfield</city>
			<empty></empty>
			<nochildren></nochildren>
		</address>
	</person>
</root>
`)

func TestXMLNode(t *testing.T) {
	node := new(Node)
	err := xml.Unmarshal(data, node)
	if err != nil {
		t.Error("Error: ", err)
	}
	content, err := xml.MarshalIndent(node, "", "    ")
	if err != nil {
		t.Error("Error: ", err)
	}
	if len(content) == 0 {
		t.Error("Error: ", err)
	}
	fmt.Println(string(content))
}
