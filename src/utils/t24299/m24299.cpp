#include "t24299/m24299.h"
QVector<double> m24299::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
