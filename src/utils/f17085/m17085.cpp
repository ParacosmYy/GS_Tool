#include "f17085/m17085.h"
QVector<double> m17085::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
