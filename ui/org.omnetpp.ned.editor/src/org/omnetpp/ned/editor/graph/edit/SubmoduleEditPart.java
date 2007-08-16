package org.omnetpp.ned.editor.graph.edit;

import java.util.List;

import org.eclipse.core.runtime.Assert;
import org.eclipse.draw2d.ConnectionAnchor;
import org.eclipse.draw2d.IFigure;
import org.eclipse.draw2d.geometry.Point;

import org.omnetpp.common.displaymodel.DisplayString;
import org.omnetpp.figures.SubmoduleFigure;
import org.omnetpp.figures.layout.SubmoduleConstraint;
import org.omnetpp.figures.misc.GateAnchor;
import org.omnetpp.ned.model.INEDElement;
import org.omnetpp.ned.model.NEDTreeUtil;
import org.omnetpp.ned.model.ex.SubmoduleNodeEx;


/**
 * Controls the submodule figure according to the model changes.
 *
 * @author rhornig
 */
public class SubmoduleEditPart extends ModuleEditPart {
    protected GateAnchor gateAnchor;

    @Override
    protected void createEditPolicies() {
        super.createEditPolicies();
    }
    /**
     * Returns a newly created Figure of this.
     */
    @Override
    protected IFigure createFigure() {
        IFigure fig = new SubmoduleFigure();
        gateAnchor = new GateAnchor(fig);
        return fig;
    }

    /**
     * Returns the Figure for this as an SubmoduleFigure.
     */
    public SubmoduleFigure getSubmoduleFigure() {
        return (SubmoduleFigure)getFigure();
    }

    /**
     * Helper function to return the model object with correct type
     */
    public SubmoduleNodeEx getSubmoduleModel() {
        return (SubmoduleNodeEx)getModel();
    }

	/**
	 * Compute the source connection anchor to be assigned based on the current mouse
	 * location and available gates.
	 * @param p current mouse coordinates
	 * @return The selected connection anchor
	 */
	@Override
    public ConnectionAnchor getConnectionAnchorAt(Point p) {
        return gateAnchor;
	}

	/**
	 * Returns a connection anchor registered for the given gate
	 */
	@Override
    public GateAnchor getConnectionAnchor(String gate) {
        return gateAnchor;
	}

    /**
     * Returns a list of connections for which this is the srcModule.
     */
    @SuppressWarnings("unchecked")
    @Override
    protected List getModelSourceConnections() {
        // get the connections from out controller parent's model
        return getCompoundModulePart().getCompoundModuleModel().getSrcConnectionsFor(getSubmoduleModel().getName());
    }

    /**
     * Returns a list of connections for which this is the destModule.
     */
    @SuppressWarnings("unchecked")
    @Override
    protected List getModelTargetConnections() {
        // get the connections from out controller parent's model
        return getCompoundModulePart().getCompoundModuleModel().getDestConnectionsFor(getSubmoduleModel().getName());
    }

    /**
     * Updates the visual aspect of this.
     */
    @Override
    protected void refreshVisuals() {
        // define the properties that determine the visual appearance
    	SubmoduleNodeEx submNode = (SubmoduleNodeEx)getModel();

    	// set module name and vector size
    	String nameToDisplay = submNode.getName();
    	// add [size] if it's a module vector
    	if (submNode.getVectorSize() != null && !"".equals(submNode.getVectorSize()))
    		nameToDisplay += "["+submNode.getVectorSize()+"]";
    	getSubmoduleFigure().setName(nameToDisplay);

    	// parse a display string, so it's easier to get values from it.
    	// for other visual properties
        DisplayString dps = submNode.getEffectiveDisplayString();

        // get the scale factor for this submodule (coming from the containing compound module's display string)
        float scale = getScale();
        // set it in the figure, so size and range indicator can use it
        getSubmoduleFigure().setScale(scale);

        // set the rest of the display properties
        getSubmoduleFigure().setDisplayString(dps);

        // show/hide the pin marker
        getSubmoduleFigure().setPinDecoration(dps.getLocation(scale) != null);

        // mark if the model is invalid
        getSubmoduleFigure().setErrorDecoration(getSubmoduleModel().hasErrorMarkers());

        // set layout constraints
        SubmoduleConstraint constraint = new SubmoduleConstraint();
        constraint.setLocation(getSubmoduleFigure().getPreferredLocation());
        constraint.setSize(getSubmoduleFigure().getPreferredSize());
        // System.out.println("constraint for " + nameToDisplay + ": " + constraint);
        Assert.isTrue(constraint.height != -1 && constraint.width != -1);
        getSubmoduleFigure().getParent().setConstraint(getSubmoduleFigure(), constraint);
    }

    @Override
    public float getScale() {
        // get the container compound module's scaling factor
        return getCompoundModulePart().getScale();
    }

    @Override
    public boolean isEditable() {
        // editable only if the parent controllers model is the same as the model's parent
        // i.e. the submodule is defined in this compound module (not inherited)
        return super.isEditable() && getParent().getModel() == ((SubmoduleNodeEx)getModel()).getCompoundModule();
    }

    @Override
    public CompoundModuleEditPart getCompoundModulePart() {
        return (CompoundModuleEditPart)getParent();
    }

    @Override
    protected INEDElement getNEDElementToOpen() {
        return getSubmoduleModel().getEffectiveTypeRef();
    }
}
