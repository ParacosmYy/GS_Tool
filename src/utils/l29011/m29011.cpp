#include "l29011/m29011.h"
QVector<double> m29011::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
