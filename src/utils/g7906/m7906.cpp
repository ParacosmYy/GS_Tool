#include "g7906/m7906.h"
QVector<double> m7906::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
