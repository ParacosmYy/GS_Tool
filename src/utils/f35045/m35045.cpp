#include "f35045/m35045.h"
QVector<double> m35045::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
