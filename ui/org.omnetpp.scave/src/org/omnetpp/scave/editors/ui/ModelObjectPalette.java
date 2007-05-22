package org.omnetpp.scave.editors.ui;

import java.util.HashMap;

import org.eclipse.emf.common.notify.AdapterFactory;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.emf.edit.ui.dnd.LocalTransfer;
import org.eclipse.emf.edit.ui.provider.AdapterFactoryLabelProvider;
import org.eclipse.jface.viewers.ILabelProvider;
import org.eclipse.jface.viewers.StructuredSelection;
import org.eclipse.swt.SWT;
import org.eclipse.swt.dnd.DND;
import org.eclipse.swt.dnd.DragSource;
import org.eclipse.swt.dnd.DragSourceEvent;
import org.eclipse.swt.dnd.DragSourceListener;
import org.eclipse.swt.dnd.Transfer;
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.events.MouseListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.widgets.ToolBar;
import org.eclipse.swt.widgets.ToolItem;
import org.omnetpp.scave.model.ScaveModelFactory;

public class ModelObjectPalette {
	private HashMap<ToolItem, EObject> toolItems = new HashMap<ToolItem, EObject>();
	private Object objectToDrag;

	public ModelObjectPalette(ToolBar toolbar, AdapterFactory adapterFactory) {
		// the following lines do something like this line from AbstractEMFModelEditor:
		// viewer.addDragSupport(dndOperations, transfers, new ViewerDragAdapter(viewer));
		int dndOperations = DND.DROP_COPY | DND.DROP_MOVE | DND.DROP_LINK;
		Transfer[] transferTypes = new Transfer[] { LocalTransfer.getInstance() };
		final DragSource dragSource = new DragSource(toolbar, dndOperations);
		dragSource.setTransfer(transferTypes);

		// modeled after EMF's ViewerDragAdapter
		dragSource.addDragListener(new DragSourceListener() {
			public void dragStart(DragSourceEvent event) {}
			public void dragFinished(DragSourceEvent event) {
				System.out.println("dragFinished()");
				LocalTransfer.getInstance().javaToNative(null, null);
			}

			public void dragSetData(DragSourceEvent event) {
				System.out.println("dragSetData()");
				if (LocalTransfer.getInstance().isSupportedType(event.dataType))
					event.data = new StructuredSelection(objectToDrag);
			}
		});
		
		ILabelProvider labelProvider = new AdapterFactoryLabelProvider(adapterFactory);
		ScaveModelFactory factory = ScaveModelFactory.eINSTANCE;

		addToolItem(toolbar, factory.createDataset(), labelProvider);
		addToolItem(toolbar, factory.createAdd(), labelProvider);
		addToolItem(toolbar, factory.createApply(), labelProvider);
		addToolItem(toolbar, factory.createCompute(), labelProvider);
		addToolItem(toolbar, factory.createBarChart(), labelProvider);
		addToolItem(toolbar, factory.createLineChart(), labelProvider);
		addToolItem(toolbar, factory.createExcept(), labelProvider);
		//XXX and more...
		
		toolbar.addMouseListener(new MouseListener() {
			public void mouseDoubleClick(MouseEvent e) {}
			public void mouseUp(MouseEvent e) {}
			public void mouseDown(MouseEvent e) {
				ToolItem toolItem = toolItemUnderMouse(e);
				objectToDrag = toolItem==null ? null : toolItems.get(toolItem);
				System.out.println("object to drag: "+objectToDrag);
			}
			private ToolItem toolItemUnderMouse(MouseEvent e) {
				for (ToolItem item : toolItems.keySet())
					if (item.getBounds().contains(e.x, e.y))
						return item;
				return null;
			}
		});

	}

	private void addToolItem(ToolBar tb, final EObject element, ILabelProvider labelProvider) {
		ToolItem toolItem = new ToolItem(tb, SWT.PUSH);
		toolItem.setImage(labelProvider.getImage(element));
		toolItem.setToolTipText("Click or drag to create "+labelProvider.getText(element));

		toolItems.put(toolItem, element);
	
		toolItem.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				System.out.println("SELECTED");
			}
		});
	}
}
