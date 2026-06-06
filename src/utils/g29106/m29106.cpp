#include "g29106/m29106.h"
QVector<double> m29106::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
