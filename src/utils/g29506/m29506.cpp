#include "g29506/m29506.h"
QVector<double> m29506::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
