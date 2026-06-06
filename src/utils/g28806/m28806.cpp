#include "g28806/m28806.h"
QVector<double> m28806::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
