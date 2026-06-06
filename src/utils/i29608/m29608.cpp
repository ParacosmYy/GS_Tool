#include "i29608/m29608.h"
QVector<double> m29608::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
