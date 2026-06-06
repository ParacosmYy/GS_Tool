#include "f9745/m9745.h"
QVector<double> m9745::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
