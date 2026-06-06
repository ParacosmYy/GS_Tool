#include "m30712/m30712.h"
QVector<double> m30712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
