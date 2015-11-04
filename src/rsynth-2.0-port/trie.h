/* trie.h
*/
typedef struct trie_s *trie_ptr;

extern void trie_insert (trie_ptr *r,char *s,void *value);
extern void *trie_lookup (trie_ptr *r,char **sp);
