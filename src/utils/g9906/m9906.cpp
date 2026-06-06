#include "g9906/m9906.h"
QVector<double> m9906::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
