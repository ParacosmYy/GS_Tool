#include "i27408/m27408.h"
QVector<double> m27408::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
