#include "m29072/m29072.h"
QVector<double> m29072::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
