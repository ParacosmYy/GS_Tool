#include "n8833/m8833.h"
QVector<double> m8833::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
