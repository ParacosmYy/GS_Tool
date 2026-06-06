#include "g27906/m27906.h"
QVector<double> m27906::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
