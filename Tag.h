/** \brief A tag type to perform tag dispatching. */
template <typename> //
struct Tag {
  /**
   * \brief Ctor.
   * \param dummy Dummy param to avoid the most vexing parse.
   */
  Tag(int /* dummy */) {}
};
