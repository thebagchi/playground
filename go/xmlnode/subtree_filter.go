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
			return result
		} else {
			return nil
		}
	}
	if filter.HasChildren() {
		// Collect all filter children and selection criteria (children with text)
		var filters []*Node
		var matches []*Node
		filter.WalkNodes(func(fc *Node) {
			filters = append(filters, fc)
			if fc.HasText() {
				matches = append(matches, fc)
			}
		})
		if len(matches) > 0 {
			// Selection mode: check that all selection criteria have matching data children
			matched := true
			for _, fc := range matches {
				found := false
				n.WalkNodes(func(nc *Node) {
					if nc.SubtreeFilter(fc) != nil {
						found = true
					}
				})
				if !found {
					matched = false
				}
			}
			if matched {
				if len(matches) == len(filters) {
					// All filter children have text: include entire content
					result.Content = n.Content
				} else {
					// Mixed: filter content based on filter children
					for _, fc := range filters {
						n.WalkNodes(func(nc *Node) {
							if filtered := nc.SubtreeFilter(fc); filtered != nil {
								result.AddChild(filtered)
							}
						})
					}
					if result.Content == nil {
						return nil
					}
				}
				return result
			} else {
				return nil
			}
		} else {
			// Filter mode: recursively filter matching children
			for _, fc := range filters {
				n.WalkNodes(func(nc *Node) {
					if filtered := nc.SubtreeFilter(fc); filtered != nil {
						result.AddChild(filtered)
					}
				})
			}
			if result.Content == nil {
				return nil
			}
		}
	} else {
		// Filter has no children content (text or nil): select the data
		result.Content = n.Content
	}
	return result
}
