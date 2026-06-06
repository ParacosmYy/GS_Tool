#include "p26315/m26315.h"
QVector<double> m26315::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
