package org.omnetpp.resources;

import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.ui.plugin.AbstractUIPlugin;
import org.omnetpp.resources.builder.NEDBuilder;
import org.osgi.framework.BundleContext;

/**
 * The main plugin class to be used in the desktop.
 */
public class NEDResourcesPlugin extends AbstractUIPlugin {

	//The shared instance.
	private static NEDResourcesPlugin plugin;

	// The actual NED cache
	private NEDResources resources = new NEDResources();

	/**
	 * The constructor.
	 */
	public NEDResourcesPlugin() {
		plugin = this;
	}

	/**
	 * This method is called upon plug-in activation
	 */
	public void start(BundleContext context) throws Exception {
		super.start(context);

		// XXX this is most probably NOT the way to archieve that
		// all NED files get parsed on startup. At minimum, this should
		// be done in the background, as a long-running operation with an 
		// IProgressMonitor...
		// Cf. quote from org.eclipse.core.runtime.Plugin: 
		//   "Note 2: This method is intended to perform simple initialization 
		//   of the plug-in environment. The platform may terminate initializers 
		//   that do not complete in a timely fashion."
		// So we should find a better way.
		NEDBuilder.runFullBuild();
	}

	/**
	 * This method is called when the plug-in is stopped
	 */
	public void stop(BundleContext context) throws Exception {
		super.stop(context);
		plugin = null;
	}

	/**
	 * Returns the shared instance.
	 *
	 * @return the shared instance.
	 */
	public static NEDResourcesPlugin getDefault() {
		return plugin;
	}

	/**
	 * Returns the NED file cache of the shared instance of the plugin
	 */
	public static NEDResources getNEDResources() {
		return plugin.resources;
	}

	/**
	 * Returns an image descriptor for the image file at the given
	 * plug-in relative path.
	 *
	 * @param path the path
	 * @return the image descriptor
	 */
	public static ImageDescriptor getImageDescriptor(String path) {
		return AbstractUIPlugin.imageDescriptorFromPlugin("org.omnetpp.resources", path);
	}
}
