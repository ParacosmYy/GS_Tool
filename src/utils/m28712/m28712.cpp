#include "m28712/m28712.h"
QVector<double> m28712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
