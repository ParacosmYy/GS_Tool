#include "a35620/m35620.h"
QVector<double> m35620::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
