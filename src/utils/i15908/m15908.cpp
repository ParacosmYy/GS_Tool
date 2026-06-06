#include "i15908/m15908.h"
QVector<double> m15908::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
