#include "h35907/m35907.h"
QVector<double> m35907::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
