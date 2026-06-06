#include "j7989/m7989.h"
QVector<double> m7989::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
