#include "s9378/m9378.h"
QVector<double> m9378::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
