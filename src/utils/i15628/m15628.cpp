#include "i15628/m15628.h"
QVector<double> m15628::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
