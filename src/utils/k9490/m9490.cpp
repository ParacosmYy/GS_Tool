#include "k9490/m9490.h"
QVector<double> m9490::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
