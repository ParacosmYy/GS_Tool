#include "i15408/m15408.h"
QVector<double> m15408::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
