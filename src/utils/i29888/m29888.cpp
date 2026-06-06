#include "i29888/m29888.h"
QVector<double> m29888::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
