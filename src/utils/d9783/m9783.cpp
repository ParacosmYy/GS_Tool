#include "d9783/m9783.h"
QVector<double> m9783::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
