#include "f7825/m7825.h"
QVector<double> m7825::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
