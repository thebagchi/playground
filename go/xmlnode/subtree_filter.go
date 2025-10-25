package xmlnode

import "maps"

func (n *Node) MatchAttributes(filter *Node) bool {
	if filter == nil || filter.Attributes == nil {
		return true
	}
	if n.Attributes == nil {
		return false
	}
	for key, value := range filter.Attributes {
		v, ok := n.Attributes[key]
		if !ok || v != value {
			return false
		}
	}
	return true
}

func (n *Node) SubtreeFilter(filter *Node) *Node {
	if n == nil || filter == nil {
		return nil
	}

	// Check if the data node matches the filter node name
	if n.Name != filter.Name {
		return nil
	}

	// Check if the data node attributes match the filter attributes
	if !n.MatchAttributes(filter) {
		return nil
	}

	// Create a result node with the same name and attributes
	result := &Node{
		Name:       n.Name,
		Attributes: make(map[string]string),
	}
	maps.Copy(result.Attributes, n.Attributes)

	if filter.HasText() {
		if n.GetText() == filter.GetText() {
			result.SetText(n.GetText())
		}
		return result
	}
	if filter.HasChildren() {
		// Filter node has children: recursively filter matching children
		filter.WalkNodes(func(fc *Node) {
			// Walk all data children and filter those with matching name
			n.WalkNodes(func(nc *Node) {
				if filtered := nc.SubtreeFilter(fc); filtered != nil {
					result.AddChild(filtered)
				}
			})
		})
	} else {
		// Filter has no children content (text or nil): select the data
		result.Content = n.Content
	}
	return result
}
