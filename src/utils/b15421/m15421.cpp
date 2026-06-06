#include "b15421/m15421.h"
QVector<double> m15421::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
