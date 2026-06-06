#include "k29490/m29490.h"
QVector<double> m29490::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
