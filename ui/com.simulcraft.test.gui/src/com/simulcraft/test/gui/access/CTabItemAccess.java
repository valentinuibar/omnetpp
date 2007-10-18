package com.simulcraft.test.gui.access;

import junit.framework.Assert;

import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.CTabItem;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.widgets.Menu;
import org.omnetpp.common.util.ReflectionUtils;

import com.simulcraft.test.gui.core.InUIThread;

public class CTabItemAccess extends ClickableWidgetAccess
{
	public CTabItemAccess(CTabItem cTabItem) {
		super(cTabItem);
	}
	
    @Override
	public CTabItem getWidget() {
		return (CTabItem)widget;
	}
    
    @InUIThread
    public boolean isClosable() {
		return (Boolean)ReflectionUtils.getFieldValue(getWidget(), "showClose");
    }
	
	@Override
	protected Menu getContextMenu() {
		return null;
	}

	@Override
	protected Point getAbsolutePointToClick() {
		return getWidget().getParent().toDisplay(getCenter(getWidget().getBounds()));
	}
	
	@InUIThread
	public void clickOnCloseIcon() {
		Assert.assertTrue("CTabItem is not closable", isClosable());
		Rectangle rect = (Rectangle)ReflectionUtils.getFieldValue(getWidget(), "closeRect");
		Assert.assertTrue("Close icon is not yet displayed", rect != null && !rect.isEmpty());
		Point center = new Point(rect.x + rect.width / 2, rect.y + rect.height / 2);
		clickAbsolute(LEFT_MOUSE_BUTTON, getWidget().getParent().toDisplay(center));
	}
}
