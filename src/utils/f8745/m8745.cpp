#include "f8745/m8745.h"
QVector<double> m8745::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
