#include "m29652/m29652.h"
QVector<double> m29652::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
