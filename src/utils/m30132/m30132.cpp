#include "m30132/m30132.h"
QVector<double> m30132::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
