#import <EtoileFoundation/EtoileFoundation.h>

/**
 * <p>Simple hash function used to using C strings as keys in <code>NSMapTable</code>s.
 * Simply takes the first few bytes of the string as an int</p>
 */
unsigned simpleStringHash(NSMapTable *table, const void *anObject);
/**
 * <p>Wrapper around <code>strcmp</code> for use in <code>NSMapTable</code>s.</p>
 */
BOOL isCStringEqual(NSMapTable *table, const void * str1, const void * str2);

/**
 * Set of callbacks for defining a simple string-keyed map.
 */
static const NSMapTableKeyCallBacks STRING_MAP_KEY_CALLBACKS = {simpleStringHash, isCStringEqual, NULL, NULL, NULL, NSNotAnIntMapKey};
