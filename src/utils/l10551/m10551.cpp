#include "l10551/m10551.h"
QVector<double> m10551::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
