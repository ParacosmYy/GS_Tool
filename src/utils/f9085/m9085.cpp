#include "f9085/m9085.h"
QVector<double> m9085::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
