#include "m19932/m19932.h"
QVector<double> m19932::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
