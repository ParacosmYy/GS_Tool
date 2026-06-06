#include "e35064/m35064.h"
QVector<double> m35064::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
