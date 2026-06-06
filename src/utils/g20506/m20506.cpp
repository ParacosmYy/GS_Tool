#include "g20506/m20506.h"
QVector<double> m20506::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
