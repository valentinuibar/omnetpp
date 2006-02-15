/*******************************************************************************
 * Copyright (c) 2001, 2004 IBM Corporation and others.
 * All rights reserved. This program and the accompanying materials 
 * are made available under the terms of the Common Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/cpl-v10.html
 * 
 * Contributors:
 *     IBM Corporation - initial API and implementation
 *******************************************************************************/
package org.omnetpp.ned.editor.graph.edit.policies;

import java.util.Iterator;
import java.util.List;

import org.eclipse.draw2d.ColorConstants;
import org.eclipse.draw2d.IFigure;
import org.eclipse.draw2d.RectangleFigure;
import org.eclipse.draw2d.XYLayout;
import org.eclipse.draw2d.geometry.Insets;
import org.eclipse.draw2d.geometry.Rectangle;
import org.eclipse.gef.EditPart;
import org.eclipse.gef.EditPolicy;
import org.eclipse.gef.GraphicalEditPart;
import org.eclipse.gef.LayerConstants;
import org.eclipse.gef.Request;
import org.eclipse.gef.RequestConstants;
import org.eclipse.gef.commands.Command;
import org.eclipse.gef.commands.CompoundCommand;
import org.eclipse.gef.editpolicies.ResizableEditPolicy;
import org.eclipse.gef.requests.ChangeBoundsRequest;
import org.eclipse.gef.requests.CreateRequest;
import org.omnetpp.ned.editor.graph.figures.ModuleFigure;
import org.omnetpp.ned.editor.graph.misc.ColorFactory;
import org.omnetpp.ned.editor.graph.misc.DesktopLayoutEditPolicy;
import org.omnetpp.ned.editor.graph.misc.MessageFactory;
import org.omnetpp.ned.editor.graph.model.Container;
import org.omnetpp.ned.editor.graph.model.CompoundModule;
import org.omnetpp.ned.editor.graph.model.NedNode;
import org.omnetpp.ned.editor.graph.model.commands.AddCommand;
import org.omnetpp.ned.editor.graph.model.commands.CloneCommand;
import org.omnetpp.ned.editor.graph.model.commands.CreateCommand;
import org.omnetpp.ned.editor.graph.model.commands.SetConstraintCommand;


public class NedLayoutEditPolicy extends DesktopLayoutEditPolicy {

    public NedLayoutEditPolicy(XYLayout layout) {
        super();
        setXyLayout(layout);
    }


    protected Command createAddCommand(EditPart child, Object constraint) {
        return null;
    }

    protected Command createAddCommand(Request request, EditPart childEditPart, Object constraint) {
        NedNode part = (NedNode) childEditPart.getModel();
        Rectangle rect = (Rectangle) constraint;

        AddCommand add = new AddCommand();
        add.setParent((Container) getHost().getModel());
        add.setChild(part);
        add.setLabel(MessageFactory.LogicXYLayoutEditPolicy_AddCommandLabelText);
        add.setDebugLabel("LogicXYEP add subpart");//$NON-NLS-1$

        SetConstraintCommand setConstraint = new SetConstraintCommand();
        setConstraint.setLocation(rect);
        setConstraint.setPart(part);
        setConstraint.setLabel(MessageFactory.LogicXYLayoutEditPolicy_AddCommandLabelText);
        setConstraint.setDebugLabel("LogicXYEP setConstraint");//$NON-NLS-1$

        return setConstraint;
    }

    /*
     * @see org.eclipse.gef.editpolicies.ConstrainedLayoutEditPolicy#createChangeConstraintCommand(org.eclipse.gef.EditPart,
     *      java.lang.Object)
     */
    protected Command createChangeConstraintCommand(EditPart child, Object constraint) {
        return null;
    }

    // called when the editpart is moved or resized
    protected Command createChangeConstraintCommand(ChangeBoundsRequest request, EditPart child,
            Object constraint) {
        // HACK for fixing issue when the model returns unspecified size (-1,-1)
        // we have to calculate the center point in that direction manually using the size info
        // from the figure directly (which knows it's size) This is the inverse transformation of
        // CenteredXYLayout's traf.
        Rectangle figureBounds = ((GraphicalEditPart)child).getFigure().getBounds();
        Rectangle modelConstraint = (Rectangle)constraint;
        if (modelConstraint.width < 0) modelConstraint.x += figureBounds.width / 2;
        if (modelConstraint.height < 0) modelConstraint.y += figureBounds.height / 2;

        // create the constraint change command 
        SetConstraintCommand cmd = new SetConstraintCommand();
        NedNode part = (NedNode) child.getModel();
        cmd.setPart(part);
        cmd.setLocation(modelConstraint);
        Command result = cmd;

//        if ((request.getResizeDirection() & PositionConstants.NORTH_SOUTH) != 0) {
//            Integer guidePos = (Integer) request.getExtendedData().get(SnapToGuides.KEY_HORIZONTAL_GUIDE);
//            if (guidePos != null) {
//                result = chainGuideAttachmentCommand(request, part, result, true);
//            } else if (part.getHorizontalGuide() != null) {
//                // SnapToGuides didn't provide a horizontal guide, but this part
//                // is attached
//                // to a horizontal guide. Now we check to see if the part is
//                // attached to
//                // the guide along the edge being resized. If that is the case,
//                // we need to
//                // detach the part from the guide; otherwise, we leave it alone.
//                int alignment = part.getHorizontalGuide().getAlignment(part);
//                int edgeBeingResized = 0;
//                if ((request.getResizeDirection() & PositionConstants.NORTH) != 0)
//                    edgeBeingResized = -1;
//                else
//                    edgeBeingResized = 1;
//                if (alignment == edgeBeingResized) result = result.chain(new ChangeGuideCommand(part, true));
//            }
//        }
//
//        if ((request.getResizeDirection() & PositionConstants.EAST_WEST) != 0) {
//            Integer guidePos = (Integer) request.getExtendedData().get(SnapToGuides.KEY_VERTICAL_GUIDE);
//            if (guidePos != null) {
//                result = chainGuideAttachmentCommand(request, part, result, false);
//            } else if (part.getVerticalGuide() != null) {
//                int alignment = part.getVerticalGuide().getAlignment(part);
//                int edgeBeingResized = 0;
//                if ((request.getResizeDirection() & PositionConstants.WEST) != 0)
//                    edgeBeingResized = -1;
//                else
//                    edgeBeingResized = 1;
//                if (alignment == edgeBeingResized)
//                    result = result.chain(new ChangeGuideCommand(part, false));
//            }
//        }
//
//        if (request.getType().equals(REQ_MOVE_CHILDREN) || request.getType().equals(REQ_ALIGN_CHILDREN)) {
//            result = chainGuideAttachmentCommand(request, part, result, true);
//            result = chainGuideAttachmentCommand(request, part, result, false);
//            result = chainGuideDetachmentCommand(request, part, result, true);
//            result = chainGuideDetachmentCommand(request, part, result, false);
//        }

        return result;
    }

    protected EditPolicy createChildEditPolicy(EditPart child) {
        ResizableEditPolicy policy = new NedResizableEditPolicy();
        return policy;
    }

    /*
     * (non-Javadoc)
     * 
     * @see org.eclipse.gef.editpolicies.LayoutEditPolicy#createSizeOnDropFeedback(org.eclipse.gef.requests.CreateRequest)
     */
    
    // this command is created when the user adds a new child by drag/resizing with a creation
    // tool selected. ie. specifying the width/length for the new child along with the location
    protected IFigure createSizeOnDropFeedback(CreateRequest createRequest) {
        IFigure figure;

        if (createRequest.getNewObject() instanceof CompoundModule)
            figure = new ModuleFigure();
        else {
            figure = new RectangleFigure();
            ((RectangleFigure) figure).setXOR(true);
            ((RectangleFigure) figure).setFill(true);
            figure.setBackgroundColor(ColorFactory.ghostFillColor);
            figure.setForegroundColor(ColorConstants.white);
        }

        addFeedback(figure);

        return figure;
    }

    protected Command getAddCommand(Request generic) {
        ChangeBoundsRequest request = (ChangeBoundsRequest) generic;
        List editParts = request.getEditParts();
        CompoundCommand command = new CompoundCommand();
        command.setDebugLabel("Add in ConstrainedLayoutEditPolicy");//$NON-NLS-1$
        GraphicalEditPart childPart;
        Rectangle r;
        Object constraint;

        for (int i = 0; i < editParts.size(); i++) {
            childPart = (GraphicalEditPart) editParts.get(i);
            r = childPart.getFigure().getBounds().getCopy();
            // convert r to absolute from childpart figure
            childPart.getFigure().translateToAbsolute(r);
            r = request.getTransformedRectangle(r);
            // convert this figure to relative
            getLayoutContainer().translateToRelative(r);
            getLayoutContainer().translateFromParent(r);
            r.translate(getLayoutOrigin().getNegated());
            constraint = getConstraintFor(r);
            command.add(createAddCommand(generic, childPart, translateToModelConstraint(constraint)));
        }
        return command.unwrap();
    }

    /**
     * Override to return the <code>Command</code> to perform an {@link
     * RequestConstants#REQ_CLONE CLONE}. By default, <code>null</code> is
     * returned.
     * 
     * @param request
     *            the Clone Request
     * @return A command to perform the Clone.
     */
    protected Command getCloneCommand(ChangeBoundsRequest request) {
        CloneCommand clone = new CloneCommand();

        clone.setParent((Container) getHost().getModel());

        Iterator i = request.getEditParts().iterator();
        GraphicalEditPart currPart = null;

        while (i.hasNext()) {
            currPart = (GraphicalEditPart) i.next();
            clone.addPart((NedNode) currPart.getModel(), (Rectangle) getConstraintForClone(currPart,
                    request));
        }


        return clone;
    }

    protected Command getCreateCommand(CreateRequest request) {
        CreateCommand create = new CreateCommand();
        create.setParent((Container) getHost().getModel());
        NedNode newPart = (NedNode) request.getNewObject();
        create.setChild(newPart);
        Rectangle constraint = (Rectangle) getConstraintFor(request);
        create.setLocation(constraint);
        create.setLabel(MessageFactory.LogicXYLayoutEditPolicy_CreateCommandLabelText);

        return create;
    }

    /*
     * (non-Javadoc)
     * 
     * @see org.eclipse.gef.editpolicies.LayoutEditPolicy#getCreationFeedbackOffset(org.eclipse.gef.requests.CreateRequest)
     */
    protected Insets getCreationFeedbackOffset(CreateRequest request) {
        if (request.getNewObject() instanceof CompoundModule)
            return new Insets(2, 0, 2, 0);
        return new Insets();
    }

    protected Command getDeleteDependantCommand(Request request) {
        return null;
    }

    /**
     * Returns the layer used for displaying feedback.
     * 
     * @return the feedback layer
     */
    protected IFigure getFeedbackLayer() {
        return getLayer(LayerConstants.SCALED_FEEDBACK_LAYER);
    }

    protected Command getOrphanChildrenCommand(Request request) {
        return null;
    }
    
}