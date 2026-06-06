#include "b10421/m10421.h"
QVector<double> m10421::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
