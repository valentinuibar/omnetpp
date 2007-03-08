package org.omnetpp.scave.charting.plotter;

import org.eclipse.swt.graphics.GC;
import org.jfree.data.xy.XYDataset;
import org.omnetpp.common.canvas.ICoordsMapping;

public interface IVectorPlotter {

	public int getNumPointsInXRange(XYDataset dataset, int series, GC gc, ICoordsMapping mapping);

	public void plot(XYDataset dataset, int series, GC gc, ICoordsMapping mapping, IChartSymbol symbol); 
}
