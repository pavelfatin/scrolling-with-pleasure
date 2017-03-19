import javax.imageio.ImageIO;
import javax.swing.*;
import javax.swing.border.EmptyBorder;
import javax.swing.plaf.nimbus.NimbusLookAndFeel;
import java.awt.*;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;

/*
  Directions for use:
  1. Press Tab a few times to transfer the focus.
  2. Press any mouse button (on empty space) to save the image.
 */
public class BufferContent {
  public static void main(String[] args) throws Exception {
    System.getProperties().setProperty("swing.volatileImageBufferEnabled", "false");
    System.getProperties().setProperty("swing.bufferPerWindow", "false");

    SwingUtilities.invokeLater(BufferContent::run);
  }

  private static void run() {
    try {
      UIManager.setLookAndFeel(new NimbusLookAndFeel());
    } catch (UnsupportedLookAndFeelException e) {
      throw new RuntimeException(e);
    }

    JFrame frame = new JFrame("Buffer content");

    JPanel panel = createMainPanel();
    panel.addMouseListener(new MouseAdapter() {
      @Override
      public void mousePressed(MouseEvent e) {
        saveBufferContent(frame.getRootPane());
      }
    });

    frame.getContentPane().add(panel);
    frame.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
    frame.setPreferredSize(new Dimension(580, 400));
    frame.pack();
    frame.setLocationRelativeTo(null);
    frame.setVisible(true);
  }

  private static void saveBufferContent(JComponent component) {
    RepaintManager manager = RepaintManager.currentManager(component);
    Image image = manager.getOffscreenBuffer(component, component.getWidth(), component.getHeight());
    write(image, "image.png");
  }

  private static JPanel createMainPanel() {
    JButton button1 = new JButton("Button 1");
    JButton button2 = new JButton("Button 2");
    JButton button3 = new JButton("Button 3");

    button1.setFont(button1.getFont().deriveFont(16.0F));
    button2.setFont(button2.getFont().deriveFont(32.0F));
    button3.setFont(button3.getFont().deriveFont(64.0F));

    button1.setAlignmentX(Component.CENTER_ALIGNMENT);
    button2.setAlignmentX(Component.CENTER_ALIGNMENT);
    button3.setAlignmentX(Component.CENTER_ALIGNMENT);

    JPanel panel = new JPanel();
    panel.setBorder(new EmptyBorder(0, 150, 0, 0));
    panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));
    panel.add(Box.createRigidArea(new Dimension(0, 50)));
    panel.add(button1);
    panel.add(Box.createRigidArea(new Dimension(0, 40)));
    panel.add(button2);
    panel.add(Box.createRigidArea(new Dimension(0, 40)));
    panel.add(button3);

    return panel;
  }

  private static void write(Image image, String fileName) {
    BufferedImage bufferedImage = toBufferedImage(image);
    try {
      ImageIO.write(bufferedImage, "png", new File(fileName));
    } catch (IOException e) {
      throw new RuntimeException(e);
    }
  }

  private static BufferedImage toBufferedImage(Image image) {
    BufferedImage result = new BufferedImage(image.getWidth(null), image.getHeight(null), BufferedImage.TYPE_INT_ARGB);
    Graphics2D graphics = result.createGraphics();
    graphics.drawImage(image, 0, 0, null);
    graphics.dispose();
    return result;
  }
}
