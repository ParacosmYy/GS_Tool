#include "t35299/m35299.h"
QVector<double> m35299::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
