#include "s7838/m7838.h"
QVector<double> m7838::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
