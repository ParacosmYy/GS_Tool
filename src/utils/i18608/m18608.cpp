#include "i18608/m18608.h"
QVector<double> m18608::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
