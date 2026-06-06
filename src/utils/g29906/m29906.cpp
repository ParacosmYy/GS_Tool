#include "g29906/m29906.h"
QVector<double> m29906::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
