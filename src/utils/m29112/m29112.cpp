#include "m29112/m29112.h"
QVector<double> m29112::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
