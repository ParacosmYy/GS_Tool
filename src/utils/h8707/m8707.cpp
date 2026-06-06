#include "h8707/m8707.h"
QVector<double> m8707::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
