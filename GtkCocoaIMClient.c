/*
 * gtkimcocoa
 * Copyright (C) 2013 Takuro Ashie <ashie@clear-code.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "GtkCocoaIMClient.h"

@implementation GtkCocoaIMClient

- (id) init
{
  [super init];
  markedRange = NSMakeRange(NSNotFound, 0);
  selectedRange = NSMakeRange(NSNotFound, 0);
  activated = NO;
  return self;
}

- (void) setGtkIMContextCocoa: (GtkIMContextCocoa *) context
{
  gtkIMContextCocoa = context;
}

- (BOOL) filterKeyDown: (NSEvent *) keyDownEvent
{
  /*
   * Since a event whose type is "NSFlagsChanged" throw exception, we ignore it.
   */
  if ([keyDownEvent type] == NSFlagsChanged)
    return NO;

  /*
   * Although Apple's reference manual says "You should not call this method
   * directly", we should do it because a NSWindow is needed to activate the
   * inputContext automatically. This NSView never have NSWindow.
   */
  if (!activated) {
    [[self inputContext] activate];
    activated = YES;
  }

  return [[self inputContext] handleEvent:keyDownEvent];
}

- (void) keyDown: (NSEvent *)theEvent
{
  [self filterKeyDown:theEvent];
}

- (NSUInteger) characterIndexForPoint: (NSPoint)aPoint
{
  return 0;
}

- (NSRect) firstRectForCharacterRange:
  (NSRange)aRange
  actualRange:(NSRangePointer)actualRange
{
  GdkRectangle location;
  NSRect rect;

  location = gtk_im_context_cocoa_get_cursor_location(gtkIMContextCocoa);
  location.y = gdk_screen_height() - location.y - location.height;
  rect = NSMakeRect(location.x, location.y,
                    location.width, location.height);

  return rect;
}

- (NSArray *) validAttributesForMarkedText
{
  return nil;
}

- (NSAttributedString *) attributedSubstringForProposedRange:
  (NSRange)aRange
  actualRange:(NSRangePointer)actualRange
{
  return nil;
}

- (BOOL) hasMarkedText
{
  const gchar *text = gtk_im_context_cocoa_get_preedit_string(gtkIMContextCocoa);

  if (text && *text)
    return YES;
  else
    return NO;
}

- (NSRange) markedRange
{
  return markedRange;
}

- (NSRange) selectedRange
{
  return selectedRange;
}

- (void) unmarkText
{
  gtk_im_context_cocoa_set_preedit_string(gtkIMContextCocoa, "", 0, 0);
}

- (void) setMarkedText: (id)aString
  selectedRange:(NSRange)newSelection
  replacementRange:(NSRange)replacementRange
{
  const char *str;

  if ([aString isKindOfClass:[NSAttributedString class]]) {
    str = [[aString string] UTF8String];
  } else {
    str = [aString UTF8String];
  }

  selectedRange = newSelection;
  if (str && *str)
    markedRange = NSMakeRange(0, g_utf8_strlen(str, -1));
  else
    markedRange = NSMakeRange(NSNotFound, 0);

  gtk_im_context_cocoa_set_preedit_string(gtkIMContextCocoa, str,
                                          selectedRange.location,
                                          selectedRange.length);
}

- (void) doCommandBySelector: (SEL)aSelector
{
  [super doCommandBySelector:aSelector];
}

- (void) insertText: (id)aString replacementRange:(NSRange)replacementRange
{
  const char *str;
    
  if ([aString isKindOfClass:[NSAttributedString class]]) {
    str = [[aString string] UTF8String];
  } else {
    str = [aString UTF8String];
  }

  if ([self hasMarkedText])
    [self unmarkText];

  g_signal_emit_by_name(gtkIMContextCocoa, "commit", str);
}


@end