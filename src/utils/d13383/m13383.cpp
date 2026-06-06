#include "d13383/m13383.h"
QVector<double> m13383::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
